#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "constants/constants.h"
#include "system/registry.h"

class RegistryTest : public ::testing::Test
{
protected:
    Registry& registry {Registry::get_instance()};
};

TEST_F(RegistryTest, ConstructorBuildsRegistryForTrainsAndYards)
{
    for (const auto &[name, code] : Constants::SYSTEMS)
    {

        auto trains{registry.get_train_registry(code)};
        auto yards{registry.get_yard_registry(code)};

        int expected_trains{};
        int expected_yard_pairs{};

        switch (code)
        {
        case Constants::System::SUBWAY:
        {
            expected_trains =
                static_cast<int>(SUB::TrainLine::COUNT) *
                static_cast<int>(SUB::Direction::COUNT) *
                (Constants::DEFAULT_TRAINS_PER_LINE / 2);

            expected_yard_pairs = static_cast<int>(SUB::TrainLine::COUNT);

            break;
        }
        case Constants::System::METRO_NORTH:
        {
            expected_trains =
                static_cast<int>(MNR::TrainLine::COUNT) *
                static_cast<int>(MNR::Direction::COUNT) *
                (Constants::DEFAULT_TRAINS_PER_LINE / 2);

            expected_yard_pairs = static_cast<int>(MNR::TrainLine::COUNT);

            break;
        }
        case Constants::System::LIRR:
        {
            expected_trains =
                static_cast<int>(LIRR::TrainLine::COUNT) *
                static_cast<int>(LIRR::Direction::COUNT) *
                (Constants::DEFAULT_TRAINS_PER_LINE / 2);

            expected_yard_pairs = static_cast<int>(LIRR::TrainLine::COUNT);

            break;
        }
        default:
        {
            FAIL() << "Unknown system code " << static_cast<int>(code);
        }
        }

        EXPECT_EQ(trains.size(), expected_trains) << "System " << static_cast<int>(code) << " should have correct train count";
        EXPECT_EQ(yards.size(), expected_yard_pairs) << "System " << static_cast<int>(code) << " should have correct yard pair count";
    }
}

TEST_F(RegistryTest, EncodesAndDecodesCorrectly)
{
    Constants::System system_code{Constants::System::SUBWAY};
    int train_line_code{5};
    int direction_code{0};
    int instance{2};

    TrainLine train_line{static_cast<SUB::TrainLine>(train_line_code)};
    Direction direction{static_cast<SUB::Direction>(direction_code)};

    int encoded_id{registry.encode(system_code, train_line_code, direction_code, instance)};

    auto decoded{registry.decode(encoded_id)};

    EXPECT_EQ(decoded.system_code, system_code);
    EXPECT_EQ(decoded.train_line, train_line);
    EXPECT_EQ(decoded.direction, direction);
    EXPECT_EQ(decoded.instance, instance);
}