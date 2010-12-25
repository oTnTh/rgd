# coding: utf-8

require 'rubygems'
require 'rake'
require 'rake/clean'
require 'rake/gempackagetask'
require 'rake/rdoctask'
require 'rake/testtask'

spec = Gem::Specification.new do |s|
  s.name = 'rgd'
  s.version = '0.3.2a'
  s.has_rdoc = true
  s.extra_rdoc_files = ['README', 'COPYING']
  s.summary = 'libgd binding for Ruby'
  s.description = s.summary
  s.author = 'oCameLo'
  s.email = ''
  s.homepage = 'http://otnth.blogspot.com'
  # s.executables = ['your_executable_here']
  s.files = %w(COPYING README Rakefile) + Dir.glob("{bin,ext,lib,spec}/**/*")
  s.require_path = "lib"
  s.bindir = "bin"
  s.extensions = 'ext/rgd/extconf.rb'
end

CLEAN.include ['pkg', '**/*.o', '**/*.log', '**/*.def', '**/Makefile', 'ext/**/*.so']
task :build => :clean do
  spec.extensions.each do |extconf|
    Dir.chdir(File.dirname(File.expand_path(extconf))) do
      unless sh "ruby #{File.basename(extconf)}"
        $stderr.puts "Failed to run extconf"
        break
      end
      
      unless sh "make"
        $stderr.puts "Failed to make"
        break
      end
    end
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
  rdoc.rdoc_dir = 'doc/rdoc' # rdoc output folder
  rdoc.options << '--line-numbers'
end

Rake::TestTask.new do |t|
  t.test_files = FileList['test/main.rb']
end
