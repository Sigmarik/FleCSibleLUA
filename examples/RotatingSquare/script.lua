local function fib_generator()
  local a, b = 0, 1
  return function()
    local cur = a
    a, b = b, a + b
    return cur
  end
end

local gen = fib_generator()
for i = 1, 10 do
  print(gen()) -- prints first 10 Fibonacci numbers starting 0,1,1,2,...
end
