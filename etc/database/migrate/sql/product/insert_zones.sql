-- Zones ���̺�

INSERT INTO dbo.Shards
  (id, name) VALUES(1, N'development_1')

-- zone_server_1
INSERT INTO dbo.Zones
  (id, shard_id, name, global_world_map_code, is_first_zone, enabled) VALUES
  (1, 1, 'zone_server_1', 184614913, 1, 1);

-- zone_server_2
INSERT INTO dbo.Zones
  (id, shard_id, name, global_world_map_code, is_first_zone, enabled) VALUES
  (2, 1, 'zone_server_2', 184614914, 0, 1);
