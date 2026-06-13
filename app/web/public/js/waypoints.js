$(document).ready(function() {
    // Keep track of the active card row globally while editing
    let $activeWaypointCard = null;
    
    // Initialize the Bootstrap 5 Modal instance manually
    const wptModal = new bootstrap.Modal($('#waypointEditModal')[0]);

    // 1. CLICK EVENT: Open Modal and Fill Data
    $(document).on('click', '.edit-waypoint-btn', function() {
        // Find the parent card containing the waypoint attributes
        $activeWaypointCard = $(this).closest('.waypoint-list-item');
        
        const wptId = $activeWaypointCard.data('wpt-id');
        const currentTitle = $activeWaypointCard.find('.waypoint-title-text').text().trim();
        const currentDesc = $activeWaypointCard.find('.waypoint-desc-text').text().trim();

        // Populate the modal input fields
        $('#modal-wpt-id').val(wptId);
        $('#modal-wpt-title').val(currentTitle);
        $('#modal-wpt-desc').val(currentDesc);
        
        // Open the popup dialog
        wptModal.show();
    });

    $(document).on('click', '.delete-waypoint-btn', function() {
        // 1. Confirm user intention before proceeding with destructive changes
        if (!confirm('Are you sure you want to permanently delete this waypoint?')) {
            return;
        }

        const $card = $(this).closest('.waypoint-list-item');
        const $container = $(this).closest('.waypoint-component-container');
        
        // Extract parameters contextually from the respective parent blocks
        const routeId = $container.data('route-id');
        const wptId = $card.data('wpt-id');

        // 2. Package parameters securely into a FormData stream payload object
        const formData = new FormData();
    
        formData.append('wpt_id', wptId);

        // 3. Dispatch execution payload request line directly to server
        $.ajax({
            url: '/api/waypoint/delete',
            type: 'POST',
            data: formData,
            processData: false, // MANDATORY: Keeps FormData structure intact
            contentType: false, // MANDATORY: Automatically generates correct multipart boundary
            success: function(response) {
                // Expected Server JSON Response: { success: true }
                if (response.success) {
                    // Remove the targeted HTML card row item directly from view
                    $card.fadeOut(300, function() {
                        $(this).remove();
                        
                        // If zero cards remain, display the empty state message placeholder row
                        if ($container.find('.waypoint-list-item').length === 0) {
                            $container.find('#waypoints-list-container').html(`
                                <div class="col-12 text-center py-4 text-muted border rounded bg-light empty-waypoints-msg">
                                    No waypoints mapped out for this route configuration yet.
                                </div>`);
                        }
                    });
                } else {
                    alert('Could not delete waypoint: ' + (response.error || 'Server processing error'));
                }
            },
            error: function() {
                alert('Connection failure trying to communicate deletion process tasks to server.');
            }
        });
    });

    // 2. CLICK EVENT: Submit Changes to Server via FormData POST
    $('#save-waypoint-modal-btn').on('click', function() {
        const $btn = $(this);
        const routeId = $('#waypoints-management-wrapper').data('route-id');
        const wptId = $('#modal-wpt-id').val();
        const newTitle = $('#modal-wpt-title').val().trim();
        const newDesc = $('#modal-wpt-desc').val().trim();

        // Validation fallback check
        if (!newTitle) {
            alert('Please enter a title.');
            return;
        }

        // Lock button to prevent double-submits
        $btn.prop('disabled', true).text('Saving...');

        // Package everything securely into FormData
        const formData = new FormData();
      
        formData.append('wpt_id', wptId);
        formData.append('name', newTitle);
        formData.append('desc', newDesc);

        $.ajax({
            url: '/api/waypoint/update',
            type: 'POST',
            data: formData,
            processData: false, // MANDATORY: Keeps FormData raw
            contentType: false, // MANDATORY: Prevents content-type text bugs
            success: function(response) {
                // Server must return JSON: { success: true }
                if (response.success) {
                    // Update the text directly on the active frontend card row
                    if ($activeWaypointCard) {
                        $activeWaypointCard.find('.waypoint-title-text').text(newTitle);
                        $activeWaypointCard.find('.waypoint-desc-text').text(newDesc);
                    }
                    
                    // Close the popup dialog
                    wptModal.hide();
                } else {
                    alert('Could not update waypoint: ' + (response.error || 'Server error'));
                }
            },
            error: function() {
                alert('Connection failure sending waypoint updates to the server.');
            },
            complete: function() {
                // Restore button state
                $btn.prop('disabled', false).text('Save Changes');
            }
        });
    });
});