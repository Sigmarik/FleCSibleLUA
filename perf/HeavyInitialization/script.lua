local function fib_rec(n)
    if n < 2 then return n end
    return fib_rec(n-1) + fib_rec(n-2)
end
print(fib_rec(20))
