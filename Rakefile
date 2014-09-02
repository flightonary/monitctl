require 'rake/clean'

CC = "gcc"
cflags = "-std=c99 -Wall"
SRCS = FileList["*.c"]
OBJS = SRCS.ext('o')

cflags += " -DCONFIG_FILE=\\\"#{ENV['config_file']}\\\"" if ENV['config_file']

CLEAN.include(OBJS)
CLOBBER.include("monitctl")

task :default => :monitctl

task :debug do
  cflags += " -DDEBUG -g"
  Rake::Task['monitctl'].invoke
end

desc "build monitctl"
file :monitctl => OBJS do |t|
  sh "#{CC} #{cflags} -o #{t.name} #{t.prerequisites.join(' ')}"
end

desc "complile all sources"
rule '.o' => ['.c'] do |t|
  sh "#{CC} #{cflags} -c #{t.source} -o #{t.name}"
end