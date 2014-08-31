require 'rake/clean'

CC = "gcc"
SRCS = FileList["*.c"]
OBJS = SRCS.ext('o')

CLEAN.include(OBJS)
CLOBBER.include("monitctl")

task :default => :monitctl

desc "build monitctl"
file :monitctl => OBJS do |t|
  sh "#{CC} -o #{t.name} #{t.prerequisites.join(' ')}"
end

desc "complile all sources"
rule '.o' => ['.c'] do |t|
  sh "#{CC} -c #{t.source} -o #{t.name}"
end