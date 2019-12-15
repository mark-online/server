#
# XML-RPC를 통해 Monitor Counter 값을 얻는다.
#
require "xmlrpc/client"

def usage
  p "Usage: #{File.basename($0)} <host> <port> <method> [interval]"
end

if __FILE__ == $0
  if ARGV.size < 3
    usage
    exit
  end

  host = ARGV[0]
  port = ARGV[1].to_i
  method = ARGV[2]
  interval = (ARGV.size == 4) ? ARGV[3].to_i : 0

  timeout = 3
  rpc = XMLRPC::Client.new2("http://#{host}:#{port}",nil, timeout)

  while true
    value = rpc.call(method)
    puts "#{Time.now}: #{method} = #{value}\n"
    break if interval == 0

    sleep interval
  end
end

