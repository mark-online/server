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
  'localhost',
  21110, 31110,
  2, 20, 0,
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
  'localhost',
  11121, 31121,
  2, 20, 50000,
  'aes', 60000,
  0, 0,
  1024, 2, 100,
  'not_used');
 
-- zone_server_2 c2s
INSERT INTO Server_Specs
  (name, host_name,
  listening_address,
  listening_port, monitor_port,
  worker_thread_count, session_pool_size, max_user_count,
  packet_cipher,cipher_key_time_limit,
  authentication_timeout, heartbeat_timeout,
  max_bytes_per_second, seconds_for_throttling, max_pendable_packet_count,
  certificate) VALUES
  ('zone_server_2', 'first',
  'localhost',
  11122, 31122,
  2, 20, 50000,
  'aes', 60000,
  0, 0,
  1024, 2, 100,
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
  'localhost',
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
  'localhost',
  11131, 31131,
  2, 20, 50000,
  'aes', 60000,
  0, 0,
  1024, 2, 100,
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
  'localhost',
  21111, 0,
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
  'localhost',
  11111, 31111,
  2, 20, 50000,
  'aes', 60000,
  0, 0,
  1024, 2, 100,
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
  21141, 0,
  2, 1, 0,
  'none', 0,
  0, 0,
  0, 0, 0,
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
  'localhost',
  11141, 31141,
  2, 20, 50000,
  'aes', 60000,
  0, 0,
  1024, 2, 100,
  'not_used');
