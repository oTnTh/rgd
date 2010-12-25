# coding: utf-8

font = 'Microsoft YaHei'
size = 40
str = 'Hello.给力否'
opts = {:fontconfig => true, :fontpath => false, :returnfontpath => true}

ret = RGD::Image.stringft(0, font, size, 0, 0, 0, str, opts)
brect = ret['brect']
x = brect[1][0] - brect[3][0] + 6
y = brect[1][1] - brect[3][1] + 6

m = RGD::Image.create(x, y);
bg = m.color_resolve('white')
fg = m.color_resolve('red')

x = 3 - brect[3][0]
y = 3 - brect[3][1]
m.stringft(fg, font, size, 0, x, y, str, opts)
m.png(File.basename(__FILE__, '.rb') + '.png')
