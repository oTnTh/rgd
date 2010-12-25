# coding: utf-8

err = 0

m = RGD::Image.create_truecolor(5, 5)
c = m.color_exact(255, 0, 255)
c2 = m.color_exact(255, 0, 255, 100)
err += 1 if c != 0xFF00FF
err += 1 if c2 != 0x64FF00FF

m = RGD::Image.create(5, 5)
c1 = m.color_allocate(255, 0 , 255)
c2 = m.color_allocate(255, 200, 0)
c3 = m.color_allocate(255, 0, 255, 100)
c1 = m.color_exact(255, 0, 255)
c2 = m.color_exact(255, 200, 0)
c3 = m.color_exact(255, 0, 255, 100)
c4 = m.color_exact(255, 34, 255, 100)
err += 1 if c1 != 0
err += 1 if c2 != 1
err += 1 if c3 != 2
err += 1 if c4 != -1

c = RGD::Image.truecolor(*m.rgba(c1))
err += 1 if c != 0xFF00FF

c = RGD::Image.truecolor(*m.rgba(c2))
err += 1 if c != 0xFFC800

a = m.rgba(c3)
a[3] = 0
c = RGD::Image.truecolor(*a)
err += 1 if c != 0xFF00FF


basename = File.basename(__FILE__, '.rb')
puts "#{basename}: #{err == 0 ? 'ok' : err}"