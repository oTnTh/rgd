# coding: utf-8

basename = File.basename(__FILE__, '.rb')
print "#{basename}: "

begin
  m = RGD::Image.create_truecolor(300, 300)

  m.antialiased = RGD::Image.truecolor(255, 255, 255, 0)
  m.line(-1, -1, -1, -1, RGD::Image::COLOR_ANTIALIASED)
  m.line(299, 299, 0, 299, RGD::Image::COLOR_ANTIALIASED)
  m.line(1, 1, 50, 50, RGD::Image::COLOR_ANTIALIASED)

  # Test for segfaults, if we reach this point, the test worked */
  puts 'ok'
rescue Exception => e
  puts 'fail'
end