# coding: utf-8

err = 0
basename = File.basename(__FILE__, '.rb')

[
  [0, 1, 10, 100, 1, 1], [1, 1, 10, 100, 2, 2],
  [2, -1, 10, 100, 1, 3], [1, -1, 10, 100, 2, 4],
  [0, 1, 100, 10, 1, 5], [1, 1, 100, 10, 2, 6],
  [2, -1, 100, 10, 1, 7], [1, -1, 100, 10, 2, 8]
].each do |a|
  idx, rx, w, h, bgd, fn = a
  
  gradient = h / (w * 2.0)
  offset = idx * w
  
  m = RGD::Image.create_truecolor(w, h)
  if bgd == 1 then
    m.filled_rectangle(0, 0, w-1, h-1, RGD::Image.truecolor(255, 255, 255, 0))
  else
    m.filled_rectangle(0, 0, w-1, h-1, RGD::Image.truecolor(255, 255, 0, 0))
  end
  
  m.antialiased = RGD::Image.truecolor(0, 0, 0, 0)
  m.line(-1, -1, -1, -1, RGD::Image::COLOR_ANTIALIASED)
  
  x1 = (rx*(-w) + 0.5).floor
  y1 = (offset-w) * gradient + 0.5
  
  x2 = (rx*w*2 + 0.5).floor
  y2 = ((offset+w*2)*gradient + 0.5).floor
  
  m.line(x1, y1, x2, y2, RGD::Image::COLOR_ANTIALIASED)
  m.line(0, m.height-1, m.width, m.height-1, 0x40FF0000)
  
  n = RGD::Image.new(basename + '_' + fn.to_s + '.png')
  err += 1 if n.compare(m) != 0
end

puts "#{basename}: #{err == 0 ? 'ok' : err}"