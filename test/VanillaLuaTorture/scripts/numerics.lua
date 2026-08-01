do
    local a = 2 + 3
    local b = 10 - 4
    local c = 3 * 4
    local d = 15 / 3          -- float division
    local e = 16 // 3         -- floor division
    local f = 17 % 5          -- modulo
    local g = 2 ^ 3           -- exponentiation
    local h = -a
    if a == 5 then print("PASS: addition") else print("FAIL: addition") end
    if b == 6 then print("PASS: subtraction") else print("FAIL: subtraction") end
    if c == 12 then print("PASS: multiplication") else print("FAIL: multiplication") end
    if d == 5.0 then print("PASS: float division") else print("FAIL: float division") end
    if e == 5 then print("PASS: floor division") else print("FAIL: floor division") end
    if f == 2 then print("PASS: modulo") else print("FAIL: modulo") end
    if g == 8 then print("PASS: exponentiation") else print("FAIL: exponentiation") end
    if h == -5 then print("PASS: unary minus") else print("FAIL: unary minus") end

    local prec = 2 + 3 * 4
    if prec == 14 then print("PASS: operator precedence") else print("FAIL: operator precedence") end
end