


namespace gideon { namespace zoneserver {

class DestroyDuelCallback
{
public:
    virtual ~DestroyDuelCallback() {}

    virtual void destory(ObjectId objectId) = 0;
};

}} // namespace gideon { namespace zoneserver {
