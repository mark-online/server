$VERBOSE = nil

# Load Rails rakefile extensions
Dir["#{File.dirname(__FILE__)}/tools/tasks/*.rake"].each { |ext| load ext } 

# default task
task :default => [:tasks]
task :tasks do
  system("rake -T")
end
