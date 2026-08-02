do
    local function add(a, b)
        return a + b
    end
    if add(3, 4) == 7 then print("PASS: function definition & call") else print("FAIL: function definition & call") end
end

-- multiple returns
do
    local function multi()
        return 1, 2, 3
    end
    local x, y, z = multi()
    if x == 1 and y == 2 and z == 3 then print("PASS: multiple returns") else print("FAIL: multiple returns") end
end

-- simple captures
do
    local function outer()
        local x = 10
        local f = function() return x end
        x = 20
        return f
    end
    if outer()() == 20 then print("PASS: simple captures") else print("FAIL: simple captures") end
end

-- recursion
do
    local function factorial(n)
        if n <= 1 then return 1 end
        return n * factorial(n - 1)
    end
    if factorial(5) == 120 then print("PASS: recursion") else print("FAIL: recursion") end
end

-- closure capturing upvalue
do
    local function counter()
        local count = 0
        return function()
            count = count + 1
            return count
        end
    end
    local c1 = counter()
    local c2 = counter()
    local v1 = c1()  -- 1
    local v2 = c1()  -- 2
    local v3 = c2()  -- 1
    if v1 == 1 and v2 == 2 and v3 == 1 then
        print("PASS: closures (upvalues)")
    else
        print("FAIL: closures (upvalues)")
    end
end