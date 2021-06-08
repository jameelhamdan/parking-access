from django.db import models
from datetime import datetime, timedelta
import math

PRICE_PER_HOUR = 3  # ILS


class ParkingSession(models.Model):
    created_on = models.DateTimeField(auto_now_add=True)
    finished_on = models.DateTimeField(null=True, blank=True)
    license_plate = models.TextField()

    def price(self):
        if self.finished_on is None:
            return None

        return math.ceil((self.finished_on - self.created_on).total_seconds() / 60.0 / 60.0) * PRICE_PER_HOUR
