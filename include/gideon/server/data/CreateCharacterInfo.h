#pragma once

#include <gideon/Common.h>
#include <gideon/cs/shared/data/EquipmentInfo.h>

namespace gideon {


/**
 * @class CreateCharacterEquipment
 * CreateCharacter 장착 장비 정보
 */
struct CreateCharacterEquipment
{
    DataCode equipCode_;
    ObjectId equipId_;
    
    CreateCharacterEquipment(DataCode equipCode = invalidEquipCode, 
        ObjectId equipId = invalidObjectId) :    
        equipCode_(equipCode),
        equipId_(equipId) {}

    bool isValid() const {
        if (isValidDataCode(equipCode_)) {
            if (! isValidEquipCode(equipCode_) || 
                ! isValidObjectId(equipId_)) {
                    return false;
            }
        }
        return true;
    }

    template <typename Stream>
    void serialize(Stream& stream) {
        stream & equipCode_ & equipId_;
    }

};

/**
 * @class CreateCharacterEquipments
 * CreateCharacter 장착 장비 정보
 */
class CreateCharacterEquipments : public sne::srpc::RArray<CreateCharacterEquipment, epCount>
{
public:
    CreateCharacterEquipments() {
        reset();
    }

    void reset() {
        fill(CreateCharacterEquipment());
    }

    bool isValid() const {
        for (const CreateCharacterEquipment& cce : *this) {
            if (! cce.isValid()) {
                return false;                
            }
        }

        if (isValidEquipCode(operator[](epTwoHands).equipCode_)) {
            if ((isValidEquipCode(operator[](epLeftHand).equipCode_)) ||
                (isValidEquipCode(operator[](epRightHand).equipCode_))) {
                    return false;
            }
        }
        return true;
    }
};

} // namespace gideon {
