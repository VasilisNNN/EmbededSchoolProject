Import("env")

import os

if not os.environ.get("GIT_CEILING_DIRECTORIES"):
    idf_path = env.PioPlatform().get_package_dir("framework-espidf")
    if not idf_path:
        raise RuntimeError("PlatformIO could not resolve the ESP-IDF package path")
    env["ENV"]["GIT_CEILING_DIRECTORIES"] = os.path.dirname(idf_path)
