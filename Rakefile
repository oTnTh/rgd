# coding: utf-8

require 'rubygems'
require 'rake'
require 'rake/clean'
require 'rake/gempackagetask'
require 'rake/rdoctask'
require 'rake/testtask'

require File.join(File.dirname(File.expand_path(__FILE__)), 'lib/rgd/version')

spec = Gem::Specification.new do |s|
  s.name = 'rgd'
  s.version = RGD::VERSION
  s.has_rdoc = true
  s.extra_rdoc_files = ['README.rdoc', 'COPYING.rdoc']
  s.summary = 'libgd binding for Ruby'
  s.description = s.summary
  s.author = 'oCameLo'
  s.email = ''
  s.homepage = 'https://github.com/oTnTh/rgd'
  # s.executables = ['your_executable_here']
  s.files = %w(BSDL COPYING.rdoc Rakefile README.rdoc) + Dir.glob("{bin,ext,lib,test}/**/*")
  s.require_path = "lib"
  s.bindir = "bin"
  if $WIN32 then
    s.platform = Gem::Platform::CURRENT
  else
    s.extensions = 'ext/rgd/extconf.rb'
  end
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
  files =['README.rdoc', 'COPYING.rdoc', 'lib/**/*.rb'] + Dir.glob("ext/**/*.c")
  rdoc.rdoc_files.add(files)
  rdoc.main = "README.rdoc" # page to start on
  rdoc.title = "RGD Docs"
  rdoc.rdoc_dir = 'doc' # rdoc output folder
  rdoc.options << '--line-numbers'
end

Rake::TestTask.new do |t|
  t.test_files = FileList['test/main.rb']
end
