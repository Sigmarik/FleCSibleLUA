local condition = true

if condition then
    print("PASS: Condition is true")
else
    print("FAIL: if expressions are not working")
end

condition = false

if condition then
    print("FAIL: if expressions are not working")
else
    print("PASS: Condition is false")
end

do
    local val = 15
    local result = ""
    if val < 10 then
        result = "low"
    elseif val < 20 then
        result = "mid"
    else
        result = "high"
    end
    if result == "mid" then print("PASS: if-elseif-else") else print("FAIL: if-elseif-else") end
end
