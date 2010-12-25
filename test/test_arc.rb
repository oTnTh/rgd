# coding: utf-8

m = RGD::Image.create_truecolor(300, 300)
m.filled_rectangle(0, 0, 299, 299, RGD::Image.truecolor('white'))
m.antialiased = RGD::Image.truecolor('black')
m.arc(300, 300, 600, 600, 0, 360, RGD::Image::COLOR_ANTIALIASED)

basename = File.basename(__FILE__, '.rb')
n = RGD::Image.new(basename + '.png')
puts "#{basename}: #{n.compare(m) == 0 ? 'ok' : 'fail'}"