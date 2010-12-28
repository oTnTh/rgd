# coding: utf-8
begin
  require File.join(File.dirname(File.expand_path(__FILE__)), '../ext/rgd/rgd')
rescue Exception => e
  require 'rubygems'
  require 'rgd'
end

$:.push '.'

if ARGV.length >= 1 && File.expand_path(__FILE__) != File.expand_path(ARGV[0]) then
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
