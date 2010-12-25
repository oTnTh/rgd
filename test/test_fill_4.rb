# coding: utf-8

m = RGD::Image.create(50, 100)
red = m.color_allocate('red')
blue = m.color_allocate('blue')
white = m.color_allocate('white')
black = m.color_allocate('black')
m.fill(0, 0, black)

m.line(20, 20, 180, 20, white)
m.line(20, 20, 20, 70, blue)
m.line(20, 70, 180, 70, red)
m.line(180, 20, 180, 45, white)
m.line(180, 70, 180, 45, red)
m.line(20, 20, 100, 45, blue)
m.line(20, 70, 100, 45, blue)
m.line(100, 45, 180, 45, red)

m.fill(21, 45, blue)
m.fill(100, 69, red)
m.fill(100, 21, white)

basename = File.basename(__FILE__, '.rb')
n = File.open(basename + '.png', 'rb').read
puts "#{basename}: #{m.png_data == n ? 'ok' : 'fail'}"