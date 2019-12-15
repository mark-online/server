UPDATE dbo.Server_Specs
    SET listening_address = 'enel.iptime.org', max_bytes_per_second = 30000

UPDATE dbo.Properties
    SET value = 100
    WHERE [key] = 'c2s.high_concurrent_session_count'
UPDATE dbo.Properties
    SET value = 200
    WHERE [key] = 'c2s.max_concurrent_session_count'
