--====================================================
-- 테스트 계정 생성
--====================================================

DECLARE @userId NVARCHAR(10)
DECLARE @password VARCHAR(128)
DECLARE @passwordSalt VARCHAR(20)
DECLARE @accountId INT
DECLARE @i INT

SET @i = 1
WHILE @i <= 2000
BEGIN
  SET @userId = 'test' + CAST(@i AS VARCHAR(10))
  -- password: 123456
  SET @password = '543bbe66eaf8dc7843e2244139e1b10028359ff417b75f5d50eb994323d9ea2d3158232f6ad3a685deda9974e967e514a1493f707c79735672f12b847d847798'
  SET @passwordSalt = 'yKcn5deok2EFKpVba56W'
  INSERT INTO dbo.Accounts (user_id, encrypted_password, password_salt, email) VALUES
	(@userId, @password, @passwordSalt, @userId + '@bsgames.co.kr')
  SET @accountId = @@IDENTITY
  SET @i = @i + 1
END
GO
