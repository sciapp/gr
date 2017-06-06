import JSON

const ETB = 0x17

x = [0.0, 0.5, 1.0]
y = [0.1, 0.25, 0.9]

try
  handle = connect("localhost", 8001)

  s = IOBuffer()
  JSON.print(s, Dict("data" => Dict("x" => x,
                                    "y" => y,
                                    "color" => [1, 0, 0.5])))
  # println(String(s.data))
  write(handle, s.data, ETB)

catch
  println("connect failed")
end
