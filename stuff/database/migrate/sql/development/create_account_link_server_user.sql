USE [master]

IF NOT EXISTS (SELECT name FROM sys.server_principals WHERE name = 'Gideon_game_development')
BEGIN
  CREATE LOGIN [Gideon_game_development] WITH PASSWORD=N'Gideon_game_development',
	DEFAULT_DATABASE=[Gideon_account_development], DEFAULT_LANGUAGE=[ÇÑ±¹¾î],
	CHECK_EXPIRATION=OFF, CHECK_POLICY=OFF		
END

USE [Gideon_account_development]
CREATE USER [Gideon_game_development] FOR LOGIN [Gideon_game_development] --WITH DEFAULT_SCHEMA=[db_owner]
EXEC sp_addrolemember N'db_owner', N'Gideon_game_development'