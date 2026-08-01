do
    local n = nil
    if n == nil then print("PASS: nil equality") else print("FAIL: nil equality") end
    if type(n) == "nil" then print("PASS: type of nil") else print("FAIL: type of nil") end

    -- nil in table removes key
    local t = {a = 1}
    t.a = nil
    if t.a == nil then print("PASS: nil removes table field") else print("FAIL: nil removes table field") end
end