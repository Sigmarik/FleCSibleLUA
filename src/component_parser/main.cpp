//! WARNING: This document is a VIBE CODED TEMPORARY solution. It is subject to be rewritten in the future.

#include <clang-c/Index.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

namespace fs = std::filesystem;

struct fieldInfo
{
    std::string name;
    std::string type;
    std::string defaultText;
};

struct structInfo
{
    std::string name;
    std::string fullName;
    std::vector<fieldInfo> fields;
};

static std::string to_std_string(CXString cxString)
{
    const char* cstr = clang_getCString(cxString);
    std::string result = cstr ? cstr : "";
    clang_disposeString(cxString);
    return result;
}

static bool extract_initializer_from_file(const std::string& filePath,
                                          unsigned startLine, unsigned startCol,
                                          unsigned endLine, unsigned endCol,
                                          std::string& outInitializer)
{
    std::ifstream fileStream(filePath);
    if (!fileStream) return false;

    std::string content((std::istreambuf_iterator<char>(fileStream)),
                        std::istreambuf_iterator<char>());

    unsigned currentLineNumber = 1;
    size_t startByteIndex = std::string::npos;
    size_t endByteIndex = std::string::npos;

    for (size_t byteIndex = 0; byteIndex < content.size(); ++byteIndex)
    {
        if (currentLineNumber == startLine && startByteIndex == std::string::npos)
            startByteIndex = byteIndex + (startCol - 1);

        if (content[byteIndex] == '\n')
        {
            if (currentLineNumber == endLine)
            {
                endByteIndex = byteIndex + (endCol - 1);
                break;
            }
            ++currentLineNumber;
        }
    }

    if (startByteIndex == std::string::npos || endByteIndex == std::string::npos || endByteIndex <= startByteIndex)
        return false;

    std::string snippet = content.substr(startByteIndex, endByteIndex - startByteIndex + 1);
    size_t equalsPos = snippet.find('=');
    if (equalsPos == std::string::npos) return false;
    size_t semicolonPos = snippet.find(';', equalsPos);
    if (semicolonPos == std::string::npos || semicolonPos <= equalsPos) return false;

    std::string rawInit = snippet.substr(equalsPos + 1, semicolonPos - equalsPos - 1);
    size_t leftTrim = rawInit.find_first_not_of(" \t\n\r");
    size_t rightTrim = rawInit.find_last_not_of(" \t\n\r");
    if (leftTrim == std::string::npos || rightTrim == std::string::npos) return false;

    outInitializer = rawInit.substr(leftTrim, rightTrim - leftTrim + 1);
    return true;
}

static bool is_type_name_allowed(const std::string& typeName)
{
    static const std::unordered_set<std::string> kAllowedTypes = {
        "int", "unsigned", "float", "double", "std::string", "bool", "flecs::entity"
    };

    return kAllowedTypes.contains(typeName);
}

static CXChildVisitResult collect_struct_fields(CXCursor cursor, CXCursor, CXClientData clientData)
{
    auto* fieldList = reinterpret_cast<std::vector<fieldInfo>*>(clientData);
    if (clang_getCursorKind(cursor) != CXCursor_FieldDecl) return CXChildVisit_Continue;
    if (clang_getCXXAccessSpecifier(cursor) != CX_CXXPublic) return CXChildVisit_Continue;

    fieldInfo field;
    field.name = to_std_string(clang_getCursorSpelling(cursor));
    field.type = to_std_string(clang_getTypeSpelling(clang_getCursorType(cursor)));
    if (!is_type_name_allowed(field.type))
    {
        std::cerr << "Warning: Ignoring field " << field.name <<
                ", type " << field.type << " is not allowed." << std::endl;
        return CXChildVisit_Continue;
    }

    CXSourceRange extent = clang_getCursorExtent(cursor);
    CXSourceLocation startLoc = clang_getRangeStart(extent);
    CXSourceLocation endLoc = clang_getRangeEnd(extent);

    CXFile sourceFile;
    unsigned startLineNumber, startColumnNumber, endLineNumber, endColumnNumber;
    clang_getSpellingLocation(startLoc, &sourceFile, &startLineNumber, &startColumnNumber, nullptr);
    clang_getSpellingLocation(endLoc, &sourceFile, &endLineNumber, &endColumnNumber, nullptr);

    std::string sourceFileName = to_std_string(clang_getFileName(sourceFile));
    std::string initializer;
    if (extract_initializer_from_file(sourceFileName, startLineNumber, startColumnNumber,
                                      endLineNumber, endColumnNumber, initializer))
        field.defaultText = std::move(initializer);

    fieldList->push_back(std::move(field));
    return CXChildVisit_Continue;
}

static std::string get_full_name(CXCursor cursor)
{
    std::string name = to_std_string(clang_getCursorSpelling(cursor));
    CXCursor parent = clang_getCursorSemanticParent(cursor);

    while (!clang_equalCursors(parent, clang_getNullCursor()))
    {
        CXCursorKind pk = clang_getCursorKind(parent);
        if (pk == CXCursor_Namespace ||
            pk == CXCursor_StructDecl ||
            pk == CXCursor_ClassDecl ||
            pk == CXCursor_EnumDecl ||
            pk == CXCursor_ClassTemplate ||
            pk == CXCursor_NamespaceAlias)
        {
            std::string p = to_std_string(clang_getCursorSpelling(parent));
            if (!p.empty()) name = p + "::" + name;
        }
        parent = clang_getCursorSemanticParent(parent);
    }

    return name;
}

static CXChildVisitResult collect_top_level_structs(CXCursor cursor, CXCursor, CXClientData clientData)
{
    auto* structList = reinterpret_cast<std::vector<structInfo>*>(clientData);
    CXCursorKind kind = clang_getCursorKind(cursor);
    if (!(kind == CXCursor_StructDecl || kind == CXCursor_ClassDecl)) return CXChildVisit_Recurse;
    if (!clang_isCursorDefinition(cursor)) return CXChildVisit_Recurse;

    clang_getCursorPrintingPolicy(cursor);

    structInfo info;
    info.name = to_std_string(clang_getCursorSpelling(cursor));
    info.fullName = get_full_name(cursor);
    clang_visitChildren(cursor, collect_struct_fields, &info.fields);
    if (!info.fields.empty()) structList->push_back(std::move(info));
    return CXChildVisit_Recurse;
}

static fs::path make_lexical_relative(const fs::path& fromPath, const fs::path& toPath)
{
    fs::path normalizedFrom = fromPath.lexically_normal();
    fs::path normalizedTo = toPath.lexically_normal();

    std::vector<fs::path> fromParts;
    std::vector<fs::path> toParts;
    for (auto const& part : normalizedFrom) if (!part.empty()) fromParts.push_back(part);
    for (auto const& part : normalizedTo) if (!part.empty()) toParts.push_back(part);

    size_t commonPrefixCount = 0;
    while (commonPrefixCount < fromParts.size() && commonPrefixCount < toParts.size() && fromParts[commonPrefixCount] ==
           toParts[commonPrefixCount])
        ++commonPrefixCount;

    fs::path resultPath;
    for (size_t idx = commonPrefixCount; idx + 1 < fromParts.size(); ++idx) resultPath /= "..";
    for (size_t idx = commonPrefixCount; idx < toParts.size(); ++idx) resultPath /= toParts[idx];
    return resultPath;
}

static void write_header_and_namespace(std::ofstream& out, const fs::path& relativeInclude)
{
    out << R"(#include <unordered_map>
#include <string>
#include <functional>
#include <variant>
#include <flecs.h>

#include )" << relativeInclude << R"(

namespace flua::cmp_info
{
using GenericComponentPtr = std::variant<int*, unsigned*, float*, double*, std::string*, bool*, flecs::entity*>;
using EntityMemberAccessor = std::function<GenericComponentPtr(flecs::entity)>;
using EntityComponentChecker = std::function<bool(flecs::entity)>;

extern const std::unordered_map<std::string, EntityMemberAccessor> ENTITY_MEMBER_MAP
{
)";
}

static void write_entity_member_map(std::ofstream& out, const std::vector<structInfo>& structs)
{
    bool wroteAny = false;
    for (auto const& structEntry : structs)
    {
        for (auto const& fieldEntry : structEntry.fields)
        {
            if (wroteAny) out << ",\n";
            wroteAny = true;
            out << "    {\"" << structEntry.name << " " << fieldEntry.name <<
                    "\", [](flecs::entity entity)->GenericComponentPtr{ return &entity.get_mut<"
                    << structEntry.fullName << ">()." << fieldEntry.name << "; }}";
        }
    }
    out << "\n};\n\n";
}

static void write_entity_component_checkers(std::ofstream& out, const std::vector<structInfo>& structs)
{
    out << "extern const std::unordered_map<std::string, EntityComponentChecker> ENTITY_COMPONENT_CHECKERS\n{\n";
    bool wroteAny = false;
    for (auto const& structEntry : structs)
    {
        if (wroteAny) out << ",\n";
        wroteAny = true;
        out << "    {\"" << structEntry.name << "\", [](flecs::entity entity){ return entity.has<" << structEntry.
                fullName << ">(); }}";
    }
    out << "\n};\n\n";
}

static void write_get_component_ids_function(std::ofstream& out, const std::vector<structInfo>& structs)
{
    out << "extern std::unordered_map<std::string, ecs_id_t> get_component_ids(const flecs::world& world)\n{\n";
    out << "    std::unordered_map<std::string, ecs_id_t> componentIds;\n";
    for (auto const& structEntry : structs)
        out << "    componentIds[\"" << structEntry.name << "\"] = flecs::component<" << structEntry.fullName <<
                ">(world).id();\n";
    out << "    return componentIds;\n}\n}\n";
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " <input.cpp> <output.cpp>\n";
        return 1;
    }

    std::string inputPathStr = argv[1];
    std::string outputPathStr = argv[2];
    fs::path inputPath(inputPathStr);
    fs::path outputPath(outputPathStr);

    CXIndex clangIndex = clang_createIndex(0, 0);
    const char* clangArgs[] = {"-x", "c++", "-std=c++17"};
    CXTranslationUnit translationUnit = nullptr;
    CXErrorCode parseResult = clang_parseTranslationUnit2(clangIndex,
                                                          inputPathStr.c_str(),
                                                          clangArgs, 3,
                                                          nullptr, 0,
                                                          CXTranslationUnit_None,
                                                          &translationUnit);
    if (parseResult != CXError_Success)
    {
        std::cerr << "Failed to parse: " << parseResult << "\n";
        clang_disposeIndex(clangIndex);
        return 1;
    }

    CXCursor rootCursor = clang_getTranslationUnitCursor(translationUnit);
    std::vector<structInfo> discoveredStructs;
    clang_visitChildren(rootCursor, collect_top_level_structs, &discoveredStructs);

    fs::path relativeInclude = make_lexical_relative(outputPath, inputPath);

    std::ofstream outFile(outputPath);
    if (!outFile)
    {
        std::cerr << "Failed to open output file: " << outputPath << "\n";
        clang_disposeTranslationUnit(translationUnit);
        clang_disposeIndex(clangIndex);
        return 1;
    }

    write_header_and_namespace(outFile, relativeInclude);
    write_entity_member_map(outFile, discoveredStructs);
    write_entity_component_checkers(outFile, discoveredStructs);
    write_get_component_ids_function(outFile, discoveredStructs);

    clang_disposeTranslationUnit(translationUnit);
    clang_disposeIndex(clangIndex);

    return 0;
}
