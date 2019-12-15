#
# 로그 관련 Rake 파일
#

require(File.join(File.dirname(__FILE__), 'properties'))

namespace :log do

  desc 'Clean log, pid & dump files'
  task :clean do
    FileUtils.rm_f(Dir.glob(LOG_SERVER_DIR + '/*.log'))
    FileUtils.rm_f(Dir.glob(LOG_SERVER_DIR + '/*.pid'))
    FileUtils.rm_f(Dir.glob(LOG_SERVER_DIR + '/*.dmp'))
  end
  
end
