#
# 문서 관련 Rake 파일
#

require(File.join(File.dirname(__FILE__), 'properties'))

namespace :doc do

  desc 'Create Doxygen files(HTML)'
  task :doxygen do
    begin
      FileUtils.mkdir(DOC_DOXYGEN_DIR + "/sne")
    rescue
    end
    
    system("doxygen.exe #{DOC_DIR}/Doxyfile");
  end
  
end
