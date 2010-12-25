# coding: utf-8

m = RGD::Image.create_truecolor(100, 100)
m.alpha_blending = true
m.save_alpha = true
bordercolor = m.color_allocate(0, 0, 0, 2)
color = m.color_allocate(0, 0, 0, 1)

m.fill_to_border(5, 5, bordercolor, color)
color = m[5, 5]

basename = File.basename(__FILE__, '.rb')
puts "#{basename} #{color == 0x1000000 ? 'ok' : color}"

