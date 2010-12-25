# coding: utf-8

m = RGD::Image.create_truecolor(100, 100)
m.filled_ellipse(50, 50, 70, 90, 0x50FFFFFF)

basename = File.basename(__FILE__, '.rb')
n = RGD::Image.new(basename + '.png')
puts "#{basename}: #{m.compare(n) == 0 ? 'ok' : 'fail'}"