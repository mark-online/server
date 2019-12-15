UPDATE dbo.Properties
    SET value = 200
    WHERE [key] = 'c2s.high_concurrent_session_count'
UPDATE dbo.Properties
    SET value = 300
    WHERE [key] = 'c2s.max_concurrent_session_count'
