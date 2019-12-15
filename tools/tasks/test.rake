#
# 테스트 관련 Rake 파일
#

require(File.join(File.dirname(__FILE__), 'properties'))

namespace :test do

  namespace :all do
    desc "Run tests for Win32 Debug & Release"
    task :win32 => ["debug:win32", "release:win32"]

    desc "Run tests for x64 Debug & Release"
    task :x64 => ["debug:x64", "release:x64"]
  end
  
  namespace :debug do
  
    desc "Run tests for Win32 Debug"
    task :win32 do
      run_test(BIN_DIR + "/Win32", "*test.debug.exe")
    end
    
    desc "Run tests for x64 Debug"
    task :x64 do
      run_test(BIN_DIR + "/x64", "*test.debug.exe")
    end
    
  end

  namespace :release do
  
    desc "Run tests for Win32 Release"
    task :win32 do
      run_test(BIN_DIR + "/Win32", "*test.exe")
    end
    
    desc "Run tests for x64 Release"
    task :x64 do
      run_test(BIN_DIR + "/x64", "*test.exe")
    end
    
  end

end

#

def run_test(bin_dir, pattern)
  files = Dir.glob(bin_dir + "/" + pattern)
  files.each { |f| system(f) }
end
