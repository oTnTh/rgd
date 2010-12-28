# coding: utf-8

require 'rubygems'
require 'rake'
require 'rake/clean'
require 'rake/gempackagetask'
require 'rake/rdoctask'
require 'rake/testtask'

spec = Gem::Specification.new do |s|
  s.name = 'rgd'
  s.version = '0.4.1a'
  s.has_rdoc = true
  s.extra_rdoc_files = ['README', 'COPYING']
  s.summary = 'libgd binding for Ruby'
  s.description = s.summary
  s.author = 'oCameLo'
  s.email = ''
  s.homepage = 'http://otnth.blogspot.com'
  # s.executables = ['your_executable_here']
  s.files = %w(BSDL COPYING Rakefile README) + Dir.glob("{bin,ext,lib,test}/**/*")
  s.require_path = "lib"
  s.bindir = "bin"
  if $WIN32 then
    s.platform = Gem::Platform::CURRENT
  else
    s.extensions = 'ext/rgd/extconf.rb'
  end
end

Rake::GemPackageTask.new(spec) do |pkg|
  pkg.gem_spec = spec
  pkg.need_tar = true
  pkg.need_zip = true
end

Rake::RDocTask.new do |rdoc|
  files =['README', 'COPYING', 'lib/**/*.rb'] + Dir.glob("ext/**/*.c")
  rdoc.rdoc_files.add(files)
  rdoc.main = "README" # page to start on
  rdoc.title = "RGD Docs"
  rdoc.rdoc_dir = 'doc' # rdoc output folder
  rdoc.options << '--line-numbers'
end

Rake::TestTask.new do |t|
  t.test_files = FileList['test/main.rb']
end
