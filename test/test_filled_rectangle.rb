# coding: utf-8

err = 0

m = RGD::Image.create_truecolor(100, 100)
m.filled_rectangle(2, 2, 80, 95, 0x50FFFFFF)

c1 = m[2, 2]
c2 = m[80, 95]
c3 = m[80, 2]
c4 = m[2, 95]
c5 = m[49, 49]

err += 1 if c1 != 0x005E5E5E
err += 1 if c2 != 0x005E5E5E
err += 1 if c3 != 0x005E5E5E
err += 1 if c4 != 0x005E5E5E

m.filled_rectangle(0, 0, 99, 99, 0x0)
m.filled_rectangle(90, 95, 2, 2, 0x50FFFFFF)

c1 = m[2, 2]
c2 = m[80, 95]
c3 = m[80, 2]
c4 = m[2, 95]
c5 = m[49, 49]

err += 1 if c1 != 0x005E5E5E
err += 1 if c2 != 0x005E5E5E
err += 1 if c3 != 0x005E5E5E
err += 1 if c4 != 0x005E5E5E

m = RGD::Image.create_truecolor(10, 10)
m.filled_rectangle(1, 1, 1, 1, 0x50FFFFFF)
c1 = m[1, 1]
c2 = m[2, 1]
c3 = m[1, 2]
c4 = m[2, 2]

err += 1 if c1 != 0x005E5E5E
err += 1 if c2 != 0x0
err += 1 if c3 != 0x0
err += 1 if c4 != 0x0

basename = File.basename(__FILE__, '.rb')
puts "#{basename}: #{err == 0 ? 'ok' : err}"