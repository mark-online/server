--====================================================
-- 테스트 계정 생성
--====================================================

DECLARE @userId NVARCHAR(10)
DECLARE @password NVARCHAR(10)
DECLARE @accountId INT
DECLARE @i INT

SET @i = 1
WHILE @i <= 2000
BEGIN
  SET @userId = 'test' + CAST(@i AS VARCHAR(10))
  SET @password = @userId
  INSERT INTO dbo.Accounts (user_id, password) VALUES
	(@userId, @password)	
  SET @accountId = @@IDENTITY
  SET @i = @i + 1
END
GO
