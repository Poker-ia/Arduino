from django.urls import path, include
from rest_framework.routers import DefaultRouter
from .views import DeviceViewSet, ValveControlViewSet, SensorReadingViewSet

router = DefaultRouter()
router.register(r'devices', DeviceViewSet)
router.register(r'valve-controls', ValveControlViewSet)
router.register(r'sensor-readings', SensorReadingViewSet)

urlpatterns = [
    path('', include(router.urls)),
]