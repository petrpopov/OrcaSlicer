#include <catch2/catch_test_macros.hpp>

#include "slic3r/Utils/OrcaCloudServiceAgent.hpp"

#include <string>

TEST_CASE("Orca cloud keychain service is fork-specific", "[OrcaCloudServiceAgent]")
{
    const std::string service = Slic3r::OrcaCloudServiceAgent::secret_store_service_name();

    CHECK(service == "OrcaSlicerPE/Auth");
    CHECK(service != "OrcaSlicer/Auth");
}
