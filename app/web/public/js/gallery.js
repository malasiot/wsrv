$(document).ready(function() {
    let currentData = []; 
    let currentIndex = 0;
    let $activeContainer = null;

    // Build structural tracking dataset for targeted gallery scope
    function syncGalleryData($container) {
        let tempArr = [];
        $container.find('.gallery-grid .gallery-item-wrapper').each(function(idx) {
            const $item = $(this);
            const $img = $item.find('.gallery-thumb');
            $img.attr('data-index', idx); // Sync DOM index order
            tempArr.push({
                id: $item.data('id'),
                url: $img.attr('src'),
                caption: $img.attr('alt') || ''
            });
        });
        return tempArr;
    }

    // Trigger File Input Click
    $(document).on('click', '.upload-trigger-btn', function() {
        $(this).closest('.gallery-component-container').find('.gallery-file-input').click();
    });

    // ACTION: UPLOAD IMAGES (POST Multipart FormData)
    $(document).on('change', '.gallery-file-input', function(e) {
        const $container = $(this).closest('.gallery-component-container');
        routeId =  $container.data('route-id'),
        wptId =  $container.data('wpt-id') || null

        const files = e.target.files;
        if (!files.length) return;

        $.each(files, function(i, file) {
            const formData = new FormData();
            formData.append('image', file);
            if (wptId)
                 formData.append('wpt', wptId);

            // Create temporary container element with a loading overlay spinner
            const tempId = 'uploading-' + Date.now() + '-' + i;
            const loadingHtml = `
                <div class="col-6 col-md-4 col-lg-3 gallery-item-wrapper upload-placeholder" id="${tempId}">
                    <div class="card h-100 bg-light d-flex align-items-center justify-content-center" style="min-height:150px;">
                        <div class="spinner-border text-primary" role="status">
                            <span class="visually-hidden">Uploading...</span>
                        </div>
                    </div>
                </div>`;
            
            $container.find('.empty-gallery-msg').remove();
            $container.find('.gallery-grid').append(loadingHtml);

            $.ajax({
                url: '/api/photos/upload/' + routeId + "/", // Change to your absolute configuration backend path
                type: 'POST',
                data: formData,
                processData: false,
                contentType: false,
                success: function(response) {
                    // Response must return structural object: { success: true, id: "db-id", url: "/path/to/img.jpg" }
                    if (response.success) {
                        const newHtml = `
                            <div class="col-6 col-md-4 col-lg-3 gallery-item-wrapper" data-id="${response.id}">
                                <div class="card h-100 cursor-pointer gallery-card">
                                    <div class="ratio ratio-1x1">
                                        <img src="${response.url}" class="card-img-top object-fit-cover gallery-thumb" alt="" data-index="0">
                                    </div>
                                    <div class="card-footer bg-light text-truncate small text-muted text-center gallery-caption-text"></div>
                                </div>
                            </div>`;
                        $(`#${tempId}`).replaceWith(newHtml);
                        updateWaypointBadgeCount($container);
                    } else {
                        $(`#${tempId}`).remove();
                        alert('Upload failed: ' + (response.error || 'Unknown error'));
                    }
                },
                error: function() {
                    $(`#${tempId}`).remove();
                    alert('An error occurred during image file upload.');
                }
            });
        });
        $(this).val(''); // Clear variable target cache input state
    });

    // Open Lightbox Component
    $(document).on('click', '.gallery-card', function() {
        $activeContainer = $(this).closest('.gallery-component-container');
        currentData = syncGalleryData($activeContainer);
        currentIndex = parseInt($(this).find('.gallery-thumb').attr('data-index'));
        
        updateLightbox();
        
        const modalElement = $activeContainer.find('.lightboxModal');
        const bsModal = bootstrap.Modal.getOrCreateInstance(modalElement);
        bsModal.show();
    });

    function updateWaypointBadgeCount($galleryContainer) {
        const wptId = $galleryContainer.data('wpt-id');
        if (!wptId) return; // Skip if this is the main route gallery

        // Count how many image items currently exist in this specific gallery grid
        const totalPhotos = $galleryContainer.find('.gallery-grid .gallery-item-wrapper:not(.upload-placeholder)').length;

        // Find the matching parent card and update its badge text layout
        const $waypointCard = $(`.waypoint-list-item[data-wpt-id="${wptId}"]`);
        $waypointCard.find('.wpt-photo-count-badge').text(totalPhotos);
    }

    function updateLightbox() {
        if (!$activeContainer || currentData.length === 0) return;
        
        const item = currentData[currentIndex];
        $activeContainer.find('.lightbox-img').attr('src', item.url).attr('alt', item.caption);
        $activeContainer.find('.lightbox-caption-input').val(item.caption);
        $activeContainer.find('.lightbox-counter').text(`${currentIndex + 1} / ${currentData.length}`);
    }

    // Directional Arrow Nav
    function navigate(direction) {
        if (currentData.length === 0) return;
        if (direction === 'next') {
            currentIndex = (currentIndex + 1) % currentData.length;
        } else {
            currentIndex = (currentIndex - 1 + currentData.length) % currentData.length;
        }
        updateLightbox();
    }

    $(document).on('click', '.next-btn', () => navigate('next'));
    $(document).on('click', '.prev-btn', () => navigate('prev'));

    $(document).on('keydown', function(e) {
        if ($('.lightboxModal.show').length === 0) return;
        if (e.key === 'ArrowRight') navigate('next');
        if (e.key === 'ArrowLeft') navigate('prev');
    });

    // ACTION: UPDATE CAPTION (POST JSON Payload)
    $(document).on('click', '.save-caption-btn', function() {
        const $btn = $(this);
        const newCaption = $activeContainer.find('.lightbox-caption-input').val().trim();
        const item = currentData[currentIndex];
        const galleryId = $activeContainer.attr('id');

        $btn.prop('disabled', true).text('Saving...');

        const formData = new FormData();
        formData.append('caption', newCaption);
        
        $.ajax({
            url: '/api/photos/caption/' + item.id, // Change to your dynamic configuration controller path
            type: 'POST',
            processData: false,
            contentType: false,
            data: formData,
            success: function(response) {
                if (response.success) {
                    item.caption = newCaption;
                    const $wrapper = $activeContainer.find(`.gallery-item-wrapper[data-id="${item.id}"]`);
                    $wrapper.find('.gallery-thumb').attr('alt', newCaption).attr('title', newCaption);
                    
                    updateLightbox();
                } else {
                    alert('Could not update caption: ' + (response.error || 'Server error'));
                }
            },
            error: function() {
                alert('Connection failure updating caption asset backend.');
            },
            complete: function() {
                $btn.prop('disabled', false).text('Save');
            }
        });
    });

    // ACTION: DELETE PHOTO (POST JSON Payload)
    $(document).on('click', '.delete-photo-btn', function() {
        if (!confirm('Are you sure you want to permanently delete this photo?')) return;

        const item = currentData[currentIndex];
        const galleryId = $activeContainer.attr('id');
        const modalElement = $activeContainer.find('.lightboxModal');
        const bsModal = bootstrap.Modal.getInstance(modalElement);

        $.ajax({
            url: '/api/photos/delete/' + item.id, // Change to your specific delete API path
            type: 'POST',
            contentType: 'application/json',
           
            success: function(response) {
                if (response.success) {
                    $activeContainer.find(`.gallery-item-wrapper[data-id="${item.id}"]`).remove();
                    currentData = syncGalleryData($activeContainer);

                    if (currentIndex >= currentData.length) {
                        currentIndex = currentData.length - 1;
                    }

                    if (currentData.length === 0) {
                        $activeContainer.find('.gallery-grid').html('<div class="col-12 text-center py-5 text-muted empty-gallery-msg">No images uploaded yet.</div>');
                        bsModal.hide();
                    } else {
                        updateLightbox();
                    }
                    updateWaypointBadgeCount($activeContainer);
                } else {
                    alert('Could not delete image: ' + (response.error || 'Server error'));
                }
            },
            error: function() {
                alert('Connection failure performing structural deletion routing tasks.');
            }
        });
    });
});