#
# DB 관련 Rake 파일
#

require(File.join(File.dirname(__FILE__), 'properties'))

namespace :db do

  desc "Migrate the database through scripts in database/migrate/changelogs."
  task :migrate do
    liquibase("update")
  end

  desc "Rolls back the last <COUNT> change sets. COUNT=n."
  task :rollback do
    count = ENV["COUNT"] ? ENV["COUNT"].to_i : 1
    liquibase("rollbackCount #{count}")
  end

  desc "Checks change log for errors."
  task :validate do
    liquibase("validate")
  end

  desc "Outputs count of unrun changesets."
  task :status do
    liquibase("status --verbose")
  end

  desc "Generate Javadoc-like documentation in database/migrate/doc."
  task :doc do
    liquibase("dbDoc doc")
    p "See #{STUFF_SERVER_DIR}/database/migrate/doc/ directory"
  end
  
end

#

def liquibase(command)
  pwd = FileUtils.pwd()
  FileUtils.cd(DB_MIGRATE_DIR)
  system("liquibase.bat #{command}");
  FileUtils.cd(pwd)
end
