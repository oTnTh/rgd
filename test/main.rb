# coding: utf-8
begin
  require 'rgd'
rescue Exception => e
  require File.join(File.dirname(File.expand_path(__FILE__)), '../ext/rgd/rgd')
end

$FONTPATH = File.join(File.dirname(File.expand_path(__FILE__)), 'DejaVuSans.ttf')

$:.push '.'

if ARGV.length >= 1 then
  load(ARGV[0])
  exit
end

Dir.chdir(File.dirname(File.expand_path(__FILE__))) do;Dir.glob('test*.rb') do |fn|
  begin
    load(fn)
  rescue Exception => e
    puts "#{fn} #{e}"
  end
end;end