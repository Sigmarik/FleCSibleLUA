-- while
do
    local i = 1
    local sum = 0
    while i <= 5 do
        sum = sum + i
        i = i + 1
    end
    if sum == 15 then print("PASS: while loop") else print("FAIL: while loop") end
end

-- repeat-until
do
    local i = 1
    local prod = 1
    repeat
        prod = prod * i
        i = i + 1
    until i > 4
    if prod == 24 then print("PASS: repeat-until loop") else print("FAIL: repeat-until loop (" .. prod .. " is not 24)") end
end

-- numeric for (with step)
do
    local sum = 0
    for i = 1, 5 do
        sum = sum + i
    end
    if sum == 15 then print("PASS: numeric for (step 1)") else print("FAIL: numeric for (step 1)") end

    local concat = ""
    for i = 5, 1, -1 do
        concat = concat .. i
    end
    if concat == "54321" then print("PASS: numeric for (negative step)") else print("FAIL: numeric for (negative step)") end
end

-- generic for with pairs (basic library, but extremely common)
-- If pairs is not available, this test will error and be caught as failure.
do
    local t = {a = 1, b = 2}
    local keys = {}
    for k in pairs(t) do
        keys[#keys + 1] = k
    end
    -- order not guaranteed, just check that we saw both keys
    local has_a, has_b = false, false
    for i = 1, #keys do
        if keys[i] == "a" then has_a = true end
        if keys[i] == "b" then has_b = true end
    end
    if has_a and has_b then print("PASS: generic for pairs") else print("FAIL: generic for pairs") end
end

-- break
do
    local sum = 0
    for i = 1, 10 do
        if i > 4 then break end
        sum = sum + i
    end
    if sum == 10 then print("PASS: break in loop") else print("FAIL: break in loop") end
end