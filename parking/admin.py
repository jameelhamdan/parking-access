from django.contrib import admin

from . import models

@admin.register(models.ParkingSession)
class ASD(admin.ModelAdmin):
    list_display = ['id', 'created_on', 'finished_on', 'license_plate', 'price']
    
    def price(self, obj):
        return f'{obj.price()} ILS'
