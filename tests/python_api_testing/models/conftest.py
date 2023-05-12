import pytest
from PIL import Image
import torchvision.transforms as transforms
from pathlib import Path


def model_location_generator_(rel_path):
    internal_weka_path = Path("/mnt/MLPerf")
    has_internal_weka = (internal_weka_path / "bit_error_tests").exists()

    if has_internal_weka:
        return Path("/mnt/MLPerf") / rel_path
    else:
        return Path("/opt/tt-metal-models") / rel_path


@pytest.fixture(scope="session")
def model_location_generator():
    return model_location_generator_


@pytest.fixture
def imagenet_sample_input(model_location_generator):
    sample_path = "/mnt/MLPerf/tt_dnn-models/samples/ILSVRC2012_val_00048736.JPEG"
    path = model_location_generator(sample_path)
    im = Image.open(path)
    im = im.resize((224, 224))
    return transforms.ToTensor()(im).unsqueeze(0)
