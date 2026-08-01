do
    -- array
    local arr = {10, 20, 30}
    if #arr == 3 and arr[1] == 10 and arr[2] == 20 and arr[3] == 30 then
        print("PASS: table array creation")
    else
        print("FAIL: table array creation")
    end

    -- record
    local rec = {name = "test", value = 42}
    if rec.name == "test" and rec["value"] == 42 then
        print("PASS: table record access")
    else
        print("FAIL: table record access")
    end

    -- array length after assignment
    arr[4] = 40
    if #arr == 4 then print("PASS: table length update") else print("FAIL: table length update") end

    -- nested table
    local nested = {inner = {x = 5}}
    if nested.inner.x == 5 then print("PASS: nested table") else print("FAIL: nested table") end
end