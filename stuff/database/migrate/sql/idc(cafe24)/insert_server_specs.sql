-- Hosts 테이블

INSERT INTO Hosts
  (name) VALUES
  ('first');

-- Server_Specs 테이블

-- database_proxy_server c2s
INSERT INTO Server_Specs
  (name, host_name,
  listening_address,
  listening_port, monitor_port,
  worker_thread_count, session_pool_size, max_user_count,
  packet_cipher,cipher_key_time_limit,
  authentication_timeout, heartbeat_timeout,
  max_bytes_per_second, seconds_for_throttling, max_pendable_packet_count,
  certificate) VALUES
  ('database_proxy_server', 'first',
  '175.125.93.37',
  23110, 33110,
  2, 200, 0,
  'none', 0,
  0, 0,
  0, 0, 0,
  'change_me');


-- zone_server_1 c2s
INSERT INTO Server_Specs
  (name, host_name,
  listening_address,
  listening_port, monitor_port,
  worker_thread_count, session_pool_size, max_user_count,
  packet_cipher,cipher_key_time_limit,
  authentication_timeout, heartbeat_timeout,
  max_bytes_per_second, seconds_for_throttling, max_pendable_packet_count,
  certificate) VALUES
  ('zone_server_1', 'first',
  '175.125.93.37',
  13121, 33121,
  2, 200, 50000,
  'aes', 60000,
  0, 0,
  4096, 1, 100,
  'not_used');

-- community_server s2s
INSERT INTO Server_Specs
  (name, host_name,
  listening_address,
  listening_port, monitor_port,
  worker_thread_count, session_pool_size, max_user_count,
  packet_cipher,cipher_key_time_limit,
  authentication_timeout, heartbeat_timeout,
  max_bytes_per_second, seconds_for_throttling, max_pendable_packet_count,
  certificate) VALUES
  ('community_server.s2s', 'first',
  '175.125.93.37',
  21131, 0,
  2, 1, 0,
  'none', 0,
  0, 0,
  0, 0, 0,
  'change_me');
 
-- community_server c2s
INSERT INTO Server_Specs
  (name, host_name,
  listening_address,
  listening_port, monitor_port,
  worker_thread_count, session_pool_size, max_user_count,
  packet_cipher,cipher_key_time_limit,
  authentication_timeout, heartbeat_timeout,
  max_bytes_per_second, seconds_for_throttling, max_pendable_packet_count,
  certificate) VALUES
  ('community_server', 'first',
  '175.125.93.37',
  11131, 31131,
  2, 200, 50000,
  'aes', 60000,
  0, 0,
  4096, 1, 100,
  'not_used');

-- login_server s2s
INSERT INTO Server_Specs
  (name, host_name,
  listening_address,
  listening_port, monitor_port,
  worker_thread_count, session_pool_size, max_user_count,
  packet_cipher,cipher_key_time_limit,
  authentication_timeout, heartbeat_timeout,
  max_bytes_per_second, seconds_for_throttling, max_pendable_packet_count,
  certificate) VALUES
  ('login_server.s2s', 'first',
  '175.125.93.37',
  23131, 33131,
  2, 1, 0,
  'none', 0,
  0, 0,
  0, 0, 0,
  'change_me');
 
-- login_server c2s
INSERT INTO Server_Specs
  (name, host_name,
  listening_address,
  listening_port, monitor_port,
  worker_thread_count, session_pool_size, max_user_count,
  packet_cipher,cipher_key_time_limit,
  authentication_timeout, heartbeat_timeout,
  max_bytes_per_second, seconds_for_throttling, max_pendable_packet_count,
  certificate) VALUES
  ('login_server', 'first',
  '175.125.93.37',
  13111, 33111,
  2, 200, 50000,
  'aes', 60000,
  0, 0,
  4096, 1, 100,
  'not_used');

-- arena_server s2s
INSERT INTO Server_Specs
  (name, host_name,
  listening_address,
  listening_port, monitor_port,
  worker_thread_count, session_pool_size, max_user_count,
  packet_cipher,cipher_key_time_limit,
  authentication_timeout, heartbeat_timeout,
  max_bytes_per_second, seconds_for_throttling, max_pendable_packet_count,
  certificate) VALUES
  ('arena_server.s2s', 'first',
  'localhost',
  23141, 0,
  2, 1, 0,
  'none', 0,
  10000, 0,
  10000, 1, 0,
  'change_me');

-- arena_server c2s
INSERT INTO Server_Specs
  (name, host_name,
  listening_address,
  listening_port, monitor_port,
  worker_thread_count, session_pool_size, max_user_count,
  packet_cipher,cipher_key_time_limit,
  authentication_timeout, heartbeat_timeout,
  max_bytes_per_second, seconds_for_throttling, max_pendable_packet_count,
  certificate) VALUES
  ('arena_server', 'first',
  'idc.bsgames.co.kr',
  13141, 33141,
  2, 2000, 50000,
  'aes', 60000,
  10000, 40000,
  10000, 1, 100,
  'not_used');


