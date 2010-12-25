# coding: utf-8

err = 0

m = RGD::Image.create_truecolor(5, 5)
c = m.color_closest(255, 0, 255)
err += 1 if c != 0xFF00FF

m = RGD::Image.create(5, 5)
c = m.color_closest(255, 0, 255)
err += 1 if c != -1

m = RGD::Image.create(5, 5)
c = m.color_allocate(255, 0, 255)
c = m.color_closest(255, 0, 255)
c = RGD::Image.truecolor(*m.rgba(c))
err += 1 if c != 0xFF00FF

m = RGD::Image.create(5, 5)
0.upto(254) { |i| c = m.color_allocate(255, 0, 0) }
c = m.color_closest(255, 0, 0)
c = RGD::Image.truecolor(*m.rgba(c))
err += 1 if c != 0xFF0000

m = RGD::Image.create(5, 5)
0.upto(255) { |i| c = m.color_allocate(255, 0, 0) }
c = m.color_closest(255, 0, 0)
c = RGD::Image.truecolor(*m.rgba(c))
err += 1 if c != 0xFF0000

basename = File.basename(__FILE__, '.rb')
puts "#{basename}: #{err == 0 ? 'ok' : err}"