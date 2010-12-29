# coding: utf-8

FILE_PATH = File.expand_path(__FILE__)
FILE_DIR = File.dirname(FILE_PATH)

begin
  require File.join(FILE_DIR, '../ext/rgd/rgd')
  require File.join(FILE_DIR, '../lib/rgd/version')
rescue Exception => e
  require 'rubygems'
  require 'rgd'
end

$:.push '.'

if ARGV.length >= 1 && FILE_PATH != File.expand_path(ARGV[0]) then
  load(ARGV[0])
  exit
end

Dir.chdir(FILE_DIR) do;Dir.glob('test*.rb') do |fn|
  begin
    load(fn)
  rescue Exception => e
    puts "#{fn} #{e}"
  end
end;end
