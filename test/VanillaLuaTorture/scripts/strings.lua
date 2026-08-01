do
    local s1 = "hello"
    local s2 = ' world'
    local s3 = s1 .. s2
    if s3 == "hello world" then print("PASS: string concatenation") else print("FAIL: string concatenation") end

    local eq1 = ("abc" == "abc")
    local eq2 = ("abc" == "def")
    if eq1 == true and eq2 == false then
        print("PASS: string equality")
    else
        print("FAIL: string equality")
    end

    local esc = "line1\nline2"
    local has_newline = false
    for i = 1, #esc do
        if string.byte(esc, i) == 10 then has_newline = true end
    end
    if has_newline then print("PASS: escape newline") else print("FAIL: escape newline") end
end