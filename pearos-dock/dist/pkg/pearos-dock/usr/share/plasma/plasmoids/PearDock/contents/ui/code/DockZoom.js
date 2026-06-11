.pragma library

function calculateZoomFactor(dockRef, iconSize, amplitud, magnification, isVertical, physicalIndex, entryProgress) {
    const baseSize = iconSize;
    const sigma = baseSize * amplitud;
    const zoom = (magnification || 0) / 100;

    if (!dockRef || zoom <= 0) {
        return 1.0;
    }

    const mousePos = dockRef.smoothMouse;

    if (mousePos < 0) {
        return 1.0;
    }

    const totalItems = dockRef.totalDockItems || 0;
    const totalSize = totalItems * baseSize;
    const availableSize = isVertical ? dockRef.height : dockRef.width;
    const centerOffset = (availableSize - totalSize) / 2;
    const iconCenter = centerOffset + (physicalIndex * baseSize) + (baseSize / 2);
    const distance = Math.abs(mousePos - iconCenter);

    if (distance > sigma * 3) {
        return 1.0;
    }

    const dynamicZoom = zoom * entryProgress;

    return 1.0 + dynamicZoom * Math.exp(-(Math.pow(distance, 2) / (2 * Math.pow(sigma, 2))));
}
