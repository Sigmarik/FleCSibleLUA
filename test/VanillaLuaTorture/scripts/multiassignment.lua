do
    local a, b, c = 1, 2, 3
    if a == 1 and b == 2 and c == 3 then print("PASS: multiple assignment") else print("FAIL: multiple assignment") end

    -- swap
    a, b = b, a
    if a == 2 and b == 1 then print("PASS: swap via multiple assignment") else print("FAIL: swap via multiple assignment") end
end