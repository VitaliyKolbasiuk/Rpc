#pragma once

#include "Interfaces.h"
#include "Protobuf/AddressBook.pb.h"
#include "ServerSession.h"
#include "EasyRpcSessionBase.h"

class RpcModel : public IRpcModel
{

    void calculate(const uint32_t operation, const double arg1, const double arg2, std::weak_ptr<ServerSession> session) override
    {
        switch (operation)
        {
            case EasyRpcFunction::plus:
                //EasyRpcSessionBase::on_plus(1, arg1, arg2, session);
                break;

            case EasyRpcFunction::minus:
                //EasyRpcSessionBase::on_minus(1, arg1, arg2, session);
                break;

//            case Operations::multiply:
//                result.set_value(arg1 * arg2);
//                break;
//
//            case Operations::divide:
//                result.set_value(arg1 / arg2);
//                break;
//
//            case Operations::sinOp:
//                break;
//
//            case Operations::cosOp:
//                break;

            default:
                failResult.set_error_message("Unexpected operation or division by zero");
                break;
        }

//        uint32_t successOrFail = failResult.error_message().empty() ? Results::success : Results::fail;
//
//        if (auto ptr = session.lock(); ptr)
//        {
//            ptr->sendEnum(successOrFail);
//
//            if (successOrFail == Results::success)
//            {
//                std::cout << "Send " << result.value() << std::endl;
//                ptr->sendProtobuf(result);
//            }
//            else
//            {
//                ptr->sendProtobuf(failResult);
//            }
//        }
    }
};