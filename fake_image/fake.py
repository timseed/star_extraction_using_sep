import numpy as np
from astropy.io import fits

# 1. Generate image dimensions and empty data
size = 400
data = np.zeros((size, size), dtype=np.float32)

# 2. Add some "fake stars" (Gaussian blobs)
np.random.seed(42)  # For reproducibility
num_stars = 20
for _ in range(num_stars):
    y, x = np.random.randint(0, size, 2)  # Random positions
    brightness = np.random.uniform(50, 200)
    width = np.random.uniform(1, 3)
    
    # Simple 2D Gaussian
    y_grid, x_grid = np.indices(data.shape)
    r_squared = (x_grid - x)**2 + (y_grid - y)**2
    star = brightness * np.exp(-r_squared / (2 * width**2))
    data += star

# 3. Add background noise
data += np.random.poisson(2, size=(size, size))

# 4. Create FITS HDU (Header/Data Unit) and save
hdu = fits.PrimaryHDU(data)
hdu.header['OBJECT'] = 'Fake Stars'
hdu.header['HISTORY'] = 'Created with Astropy'

# Write to file, overwrite if it exists
hdu.writeto('fake_stars.fits', overwrite=True)
print("Saved fake_stars.fits")
