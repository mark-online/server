#
# 빌드 관련 Rake 파일
#

require(File.join(File.dirname(__FILE__), 'properties'))

# TODO: reflection을 통한 코드 중복 제거

namespace :build do

  namespace :all do
    desc "Build Win32 All Projects"
    task :win32 => ["debug:win32", "release:win32"]
	
    desc "Build x64 All Projects"
    task :x64 => ["debug:x64", "release:x64"]

    desc "Build Win32/x64 All Projects"
    task :all => ["debug:win32", "release:win32", "debug:x64", "release:x64"]
  end
  
  namespace :debug do
    desc "Build Win32 Debug Projects"
	task :win32 do
	  build(BUILD_DIR, "Debug", "Win32");
	  build(BUILD_DIR, "StaticMtDebug", "Win32");
	end

    desc "Build x64 Debug Projects"
	task :x64 do
	  build(BUILD_DIR, "Debug", "x64");
	  build(BUILD_DIR, "StaticMtDebug", "x64");
	end
  end

  namespace :release do
    desc "Build Win32 Release Projects"
	task :win32 do
	  build(BUILD_DIR, "Release", "Win32");
	  build(BUILD_DIR, "StaticMtRelease", "Win32");
	end

    desc "Build x64 Release Projects"
	task :x64 do
	  build(BUILD_DIR, "Release", "x64");
	  build(BUILD_DIR, "StaticMtRelease", "x64");
	end
  end

end


namespace :clean do

  namespace :all do
    desc "Clean Win32 All Projects"
    task :win32 => ["debug:win32", "release:win32"]
	
    desc "Clean x64 All Projects"
    task :x64 => ["debug:x64", "release:x64"]

    desc "Clean Win32/x64 All Projects"
    task :all => ["debug:win32", "release:win32", "debug:x64", "release:x64"]
  end
  
  namespace :debug do
    desc "Clean Win32 Debug Projects"
	task :win32 do
	  clean(BUILD_DIR, "Debug", "Win32");
	end

    desc "Clean x64 Debug Projects"
	task :x64 do
	  clean(BUILD_DIR, "Debug", "x64");
	end
  end

  namespace :release do
    desc "Clean Win32 Release Projects"
	task :win32 do
	  clean(BUILD_DIR, "Release", "Win32");
	end

    desc "Clean x64 Release Projects"
	task :x64 do
	  clean(BUILD_DIR, "Release", "x64");
	end
  end

end


# "Rebuild" 명령을 직접 사용하지 않고, "Clean", "Build" 순서로 실행한다.
namespace :rebuild do

  namespace :all do
    desc "Rebuild Win32 All Projects"
    task :win32 => ["debug:win32", "release:win32"]
	
    desc "Rebuild x64 All Projects"
    task :x64 => ["debug:x64", "release:x64"]

    desc "Rebuild Win32/x64 All Projects"
    task :all => ["debug:win32", "release:win32", "debug:x64", "release:x64"]
  end
  
  namespace :debug do
    desc "Rebuild Win32 Debug Projects"
	task :win32 => ["clean:debug:win32", "build:debug:win32"]

    desc "Rebuild x64 Debug Projects"
	task :x64 => ["clean:debug:x64", "build:debug:x64"]
  end

  namespace :release do
    desc "Rebuild Win32 Release Projects"
	task :win32 => ["clean:release:win32", "build:release:win32"]

    desc "Rebuild x64 Release Projects"
	task :x64 => ["clean:release:x64", "build:release:x64"]
  end

end

#

def build(bin_dir, config, platform)
  run(bin_dir, "Build", config, platform)
end

def clean(bin_dir, config, platform)
  run(bin_dir, "Clean", config, platform)
end

def run(bin_dir, command, config, platform)
  pwd = FileUtils.pwd()
  FileUtils.cd(bin_dir)
  system("_build.bat #{command} #{config} #{platform}");
  FileUtils.cd(pwd)
end
