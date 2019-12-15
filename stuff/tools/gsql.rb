require 'win32ole'

# http://rubyonwindows.blogspot.com/2007/03/ruby-ado-and-sqlserver.html
class SqlServer
    # This class manages database connection and queries
    attr_accessor :connection, :data, :fields

    def initialize
        @connection = nil
        @data = nil
    end

    def open(address, database, userId, password)
        # Open ADO connection to the SQL Server database
        connection_string =  "Provider=SQLOLEDB.1;"
        connection_string << "Persist Security Info=False;"
        connection_string << "User ID=" << userId << ";"
        connection_string << "password=" << password << ";"
        connection_string << "Initial Catalog=" << database << ";"
        connection_string << "Data Source=" << address << ";"
        connection_string << "Network Library=dbmssocn"
        @connection = WIN32OLE.new('ADODB.Connection')
        @connection.Open(connection_string)
    end

    def query(sql)
        # Create an instance of an ADO Recordset
        recordset = WIN32OLE.new('ADODB.Recordset')
        # Open the recordset, using an SQL statement and the
        # existing ADO connection
        recordset.Open(sql, @connection)
        # Create and populate an array of field names
        @fields = []
        recordset.Fields.each do |field|
            @fields << field.Name
        end
        begin
            # Move to the first record/row, if any exist
            recordset.MoveFirst
            # Grab all records
            @data = recordset.GetRows
        rescue
            @data = []
        end
        recordset.Close
        # An ADO Recordset's GetRows method returns an array 
        # of columns, so we'll use the transpose method to 
        # convert it to an array of rows
        @data = @data.transpose
    end

    def close
        @connection.Close
    end
end

def usage
  p "Usage: #{File.basename($0)} <server-address> <database> <user-name> <password> <sql>"
end


# main
if __FILE__ == $0
    if ARGV.size < 5
        usage
        exit
    end

    db = SqlServer.new
    db.open(ARGV[0], ARGV[1], ARGV[2], ARGV[3])
    db.query(ARGV[4])
    p db.fields
    p db.data
    db.close
end

# example
#>ruby gsql.rb bsgamesdb.cafe24.com bsgamesdb bsgamesdb Rkakstnaltn59 "SELECT GETDATE()"
#>ruby gsql.rb bsgamesdb.cafe24.com bsgamesdb bsgamesdb Rkakstnaltn59 "exec update_arena_rank 1"
#>ruby gsql.rb bsgamesdb.cafe24.com bsgamesdb bsgamesdb Rkakstnaltn59 "exec update_arena_rank 3"