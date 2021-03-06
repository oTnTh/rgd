# coding: utf-8

m = RGD::Image.create(150, 150)
t = RGD::Image.create_truecolor(36, 36)

t_white = t.color_allocate('white')
t_black = t.color_allocate('black')
m_white = m.color_allocate('white')
m_black = m.color_allocate('black')

t.fill(0, 0, t_white)
t.transparent = t_black
m.transparent = m_black

x = 0
while x < 36
  y = 0
  while y < 36
    t[x, y] = t_black
    y += 2
  end
  x += 2
end

m.tile = t
m.rectangle(9, 9, 139, 139, m_black)
m.line(9, 9, 139, 139, m_black)
m.fill(11, 12, RGD::Image::COLOR_TILED)

m.fill(0, 0, 0xFFFFFF)
m.fill(0, 0, 0xFFFFFF)

basename = File.basename(__FILE__, '.rb')
n = File.open(basename + '.png', 'rb').read
puts "#{basename}: #{m.png_data == n ? 'ok' : 'fail'}"