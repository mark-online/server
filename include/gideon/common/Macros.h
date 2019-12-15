#pragma once


#ifdef GIDEON_SERVER_DLL
#   ifdef GIDEON_SERVER_EXPORT
#       define GIDEON_SERVER_API __declspec(dllexport)
#   else
#       define GIDEON_SERVER_API __declspec(dllimport)
#   endif
#else
#   define GIDEON_SERVER_API
#endif // GIDEON_SERVER_DLL
