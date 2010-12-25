# coding: utf-8

err = 0

pos = RGD::Image::MAX_COLORS

m = RGD::Image.create(1, 1)
m.transparent = pos
err += 1 if m.transparent == pos

pos = -2
m.transparent = pos
err += 1 if m.transparent == pos

basename = File.basename(__FILE__, '.rb')
puts "#{basename}: #{err == 0 ? 'ok' : err}"