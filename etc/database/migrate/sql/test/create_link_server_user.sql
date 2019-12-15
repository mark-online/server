USE [master]

IF NOT EXISTS (SELECT name FROM sys.server_principals WHERE name = 'Gideon_game_test')
BEGIN
  CREATE LOGIN [Gideon_game_test] WITH PASSWORD=N'Gideon_game_test',
	DEFAULT_DATABASE=[Gideon_game_test], DEFAULT_LANGUAGE=[ÇÑ±¹¾î],
	CHECK_EXPIRATION=OFF, CHECK_POLICY=OFF
	USE [Gideon_account_test]
	
END

USE [Gideon_account_test]
CREATE USER [Gideon_game_test] FOR LOGIN [Gideon_game_test] --WITH DEFAULT_SCHEMA=[db_owner]	

EXEC sp_addrolemember N'db_owner', N'Gideon_game_test'