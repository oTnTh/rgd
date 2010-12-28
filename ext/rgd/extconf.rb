# coding: utf-8
require 'mkmf'

if Config::CONFIG['target_os'] =~ /mingw32/i || Config::CONFIG['target_os'] =~ /mswin32/i then
  $CFLAGS << ' -DWIN32 '
  $CFLAGS << ' -DBGDWIN32 ' if with_config('static')
end

abort 'libgd is missing.' unless have_library('gd')
have_func('gdImageCreateFromBmp', 'gd.h')
['z', 'png', 'jpeg', 'Xpm', 'freetype', 'fontconfig'].each do |lib|
  have_library(lib)
end

create_makefile("rgd/#{RUBY_VERSION.sub(/\.\d+$/, '')}/rgd")
